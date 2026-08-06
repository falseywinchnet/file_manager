#include "showcase_controls.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
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

class LayoutPanel : public Panel {
public:
    LayoutPanel(StableId stable_id, Size design_size)
        : Panel(std::move(stable_id)), design_size_(design_size) {}

    void add_at(Control::Ptr child, Rect design_bounds) {
        items_.push_back({child, design_bounds});
        add_child(std::move(child));
    }

    void arrange(Rect final_bounds) override {
        const double scale_x = design_size_.width <= 0.0
            ? 1.0 : final_bounds.width / design_size_.width;
        const double scale_y = design_size_.height <= 0.0
            ? 1.0 : final_bounds.height / design_size_.height;
        for (const Item& item : items_) {
            if (const Control::Ptr child = item.control.lock()) {
                child->set_requested_bounds({item.bounds.x * scale_x,
                                             item.bounds.y * scale_y,
                                             item.bounds.width * scale_x,
                                             item.bounds.height * scale_y});
            }
        }
        Panel::arrange(final_bounds);
    }

private:
    struct Item final {
        Control::WeakPtr control;
        Rect bounds;
    };

    Size design_size_;
    std::vector<Item> items_;
};

class LayoutGroup final : public GroupBox {
public:
    LayoutGroup(StableId stable_id, std::string title, Size design_size)
        : GroupBox(std::move(stable_id), std::move(title)),
          design_size_(design_size) {}

    void add_at(Control::Ptr child, Rect design_bounds) {
        items_.push_back({child, design_bounds});
        add_child(std::move(child));
    }

    void arrange(Rect final_bounds) override {
        const double scale_x = design_size_.width <= 0.0
            ? 1.0 : final_bounds.width / design_size_.width;
        const double scale_y = design_size_.height <= 0.0
            ? 1.0 : final_bounds.height / design_size_.height;
        for (const Item& item : items_) {
            if (const Control::Ptr child = item.control.lock()) {
                child->set_requested_bounds({item.bounds.x * scale_x,
                                             item.bounds.y * scale_y,
                                             item.bounds.width * scale_x,
                                             item.bounds.height * scale_y});
            }
        }
        GroupBox::arrange(final_bounds);
    }

private:
    struct Item final {
        Control::WeakPtr control;
        Rect bounds;
    };

    Size design_size_;
    std::vector<Item> items_;
};

enum class SurfaceKind : std::uint8_t {
    header,
    sidebar,
    page,
    status,
    animation,
};

class Surface final : public LayoutPanel {
public:
    Surface(StableId stable_id, Size design_size, SurfaceKind kind)
        : LayoutPanel(std::move(stable_id), design_size), kind_(kind) {
        set_background(kind == SurfaceKind::page || kind == SurfaceKind::animation
                           ? canvas : paper);
    }

    void on_paint(Painter& painter, Rect damage) override {
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        switch (kind_) {
        case SurfaceKind::header:
            painter.fill_rect(bounds, shell_blue);
            painter.fill_rect({0.0, 0.0, bounds.width, 4.0}, shell_blue_light);
            painter.fill_rect({0.0, bounds.height - 2.0, bounds.width, 2.0},
                              shell_blue_dark);
            break;
        case SurfaceKind::sidebar:
            painter.fill_rect(bounds, Color::rgba(220, 229, 237));
            painter.draw_line({bounds.width - 1.0, 0.0},
                              {bounds.width - 1.0, bounds.height}, rule, 1.0);
            break;
        case SurfaceKind::page:
            painter.fill_rect(bounds, canvas);
            break;
        case SurfaceKind::status:
            painter.fill_rect(bounds, Color::rgba(225, 232, 238));
            painter.draw_line({0.0, 0.0}, {bounds.width, 0.0}, rule, 1.0);
            break;
        case SurfaceKind::animation:
            painter.fill_rect(bounds, paper);
            painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                                 std::max(0.0, bounds.height - 1.0)}, rule, 1.0);
            break;
        }
        static_cast<void>(damage);
    }

private:
    SurfaceKind kind_;
};

class EasingBoard final : public Control {
public:
    explicit EasingBoard(StableId stable_id)
        : Control(std::move(stable_id)) {}

    void set_paused(bool paused) {
        require_mutable();
        if (paused_ == paused) {
            return;
        }
        paused_ = paused;
        if (paused_) {
            frames_.disconnect();
        } else {
            start_ = FrameClock::now();
            register_frames();
        }
        invalidate(Dirty::paint | Dirty::semantics);
    }

    [[nodiscard]] bool paused() const noexcept { return paused_; }

    void on_frame(FrameTime now) override {
        const double elapsed = std::chrono::duration<double>(now - start_).count();
        phase_ = std::fmod(std::max(0.0, elapsed) / 2.4, 1.0);
    }

    void on_paint(Painter& painter, Rect) override {
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        painter.fill_rect(bounds, paper);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)}, rule, 1.0);
        painter.draw_text_utf8({18.0, 25.0},
                               "TIMELINE AND EASING LAB",
                               {FontRole::control, 13.0, 700, false}, shell_blue_dark);
        painter.draw_text_utf8({bounds.width - 120.0, 25.0},
                               paused_ ? "PAUSED" : "LIVE · 60 Hz",
                               {FontRole::control, 11.0, 600, false},
                               paused_ ? orange : green);

        constexpr std::array<std::pair<EasingCurve, std::string_view>, 8> curves{{
            {EasingCurve::linear, "Linear"},
            {EasingCurve::ease_in, "Ease in"},
            {EasingCurve::ease_out, "Ease out"},
            {EasingCurve::ease_in_out, "Ease in/out"},
            {EasingCurve::smooth_step, "Smooth step"},
            {EasingCurve::back_out, "Back out"},
            {EasingCurve::bounce_out, "Bounce"},
            {EasingCurve::elastic_out, "Elastic"},
        }};
        const double directed = phase_ < 0.5 ? phase_ * 2.0 : (1.0 - phase_) * 2.0;
        for (std::size_t index = 0; index < curves.size(); ++index) {
            const double y = 48.0 + static_cast<double>(index) * 34.0;
            painter.draw_text_utf8({18.0, y + 15.0}, curves[index].second,
                                   {FontRole::content, 11.0, 400, false}, muted);
            const double track_x = 112.0;
            const double track_width = std::max(40.0, bounds.width - 140.0);
            painter.fill_rect({track_x, y + 8.0, track_width, 3.0},
                              Color::rgba(210, 219, 227));
            const double eased = std::clamp(
                apply_easing(curves[index].first, directed), -0.08, 1.08);
            const double x = track_x + eased * std::max(0.0, track_width - 14.0);
            const Color color = index < 3U ? accent : index < 6U ? violet : orange;
            painter.fill_rect({x, y + 2.0, 14.0, 14.0}, color);
            painter.stroke_rect({x + 0.5, y + 2.5, 13.0, 13.0},
                                shell_blue_dark, 1.0);
        }
    }

protected:
    void on_attached_to_window() override {
        Control::on_attached_to_window();
        start_ = FrameClock::now();
        register_frames();
    }

    void on_detached_from_window() noexcept override {
        frames_.disconnect();
        Control::on_detached_from_window();
    }

private:
    void register_frames() {
        if (window() == nullptr || paused_) {
            return;
        }
        constexpr FrameInterval interval = 16ms;
        frames_ = window()->activate_surface(
            shared_from_this(), interval, FrameClock::now() + interval);
    }

    FrameRequestToken frames_;
    FrameTime start_{};
    double phase_{};
    bool paused_{};
};

class DiagnosticsCard final : public Control {
public:
    explicit DiagnosticsCard(StableId stable_id)
        : Control(std::move(stable_id)) {}

    void on_paint(Painter& painter, Rect) override {
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        painter.fill_rect(bounds, Color::rgba(28, 39, 50));
        painter.fill_rect({0.0, 0.0, 5.0, bounds.height}, green);
        painter.draw_text_utf8({16.0, 24.0}, "RETAINED RUNTIME",
                               {FontRole::control, 12.0, 700, false},
                               Color::rgba(226, 238, 246));
        if (window() == nullptr) {
            return;
        }
        const MetricsSnapshot metrics = window()->metrics_snapshot();
        char line[256]{};
        std::snprintf(line, sizeof(line),
                      "%llu controls   %llu paints   %llu chunks reused   %llu inputs",
                      static_cast<unsigned long long>(metrics.control_count),
                      static_cast<unsigned long long>(metrics.paint_passes),
                      static_cast<unsigned long long>(metrics.display_chunks_reused),
                      static_cast<unsigned long long>(metrics.input_events));
        painter.draw_text_utf8({16.0, 48.0}, line,
                               {FontRole::content, 11.0, 400, false},
                               Color::rgba(175, 203, 221));
        std::snprintf(line, sizeof(line),
                      "%llu active surfaces   %llu focus scopes   damage %.0f px",
                      static_cast<unsigned long long>(metrics.active_surface_count),
                      static_cast<unsigned long long>(metrics.focus_scope_depth),
                      metrics.painted_damage_area);
        painter.draw_text_utf8({16.0, 68.0}, line,
                               {FontRole::content, 11.0, 400, false},
                               Color::rgba(175, 203, 221));
    }
};

class DrawingEffectsBoard final : public Control {
public:
    explicit DrawingEffectsBoard(StableId stable_id)
        : Control(std::move(stable_id)) {
        set_accessible_name("Renderer-neutral drawing primitive composition");
        set_accessible_description(
            "Nested clipping, translation, fills, strokes, lines, and text");
    }

    void on_paint(Painter& painter, Rect) override {
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        painter.fill_rect(bounds, Color::rgba(26, 39, 52));
        painter.fill_rect({0.0, 0.0, bounds.width, 4.0}, accent);
        painter.draw_text_utf8({18.0, 27.0}, "RETAINED PAINTER COMPOSITION",
                               {FontRole::control, 12.0, 700, false},
                               Color::rgba(230, 240, 247));
        painter.draw_text_utf8({18.0, 48.0},
                               "save · clip · translate · fill · stroke · line · UTF-8",
                               {FontRole::content, 10.0, 400, false},
                               Color::rgba(164, 192, 211));

        painter.save();
        painter.clip_rect({18.0, 62.0, std::max(0.0, bounds.width - 36.0),
                           std::max(0.0, bounds.height - 78.0)});
        painter.translate({18.0, 62.0});
        const double width = std::max(1.0, bounds.width - 36.0);
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
    }

    [[nodiscard]] bool hit_test_local(Point) const override { return false; }

    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override {
        SemanticDescriptor descriptor;
        descriptor.role = SemanticRole::image;
        descriptor.name = accessible_name();
        descriptor.description = accessible_description();
        descriptor.exposed = true;
        return descriptor;
    }
};

class TimerMotionBoard final : public Panel {
public:
    explicit TimerMotionBoard(StableId stable_id) : Panel(std::move(stable_id)) {
        set_background(Color::rgba(247, 249, 251));
        set_border_style(BorderStyle::line);
    }

    void initialize_control_tree() {
        target_ = make_control<Button>(
            StableId(std::string(stable_id().value()) + ".target"),
            "Moving tooltip target");
        target_->set_visual_style(ButtonVisualStyle::accent);
        target_->set_style(showcase_style());
        add_child(target_);
    }

    void set_phase(double phase) {
        require_mutable();
        phase_ = std::clamp(phase, 0.0, 1.0);
        invalidate(Dirty::arrange | Dirty::paint | Dirty::semantics);
    }

    [[nodiscard]] std::shared_ptr<Button> target() const noexcept { return target_; }

    void arrange(Rect final_bounds) override {
        const double travel = std::max(0.0, final_bounds.width - 196.0);
        const double eased = 0.5 - std::cos(phase_ * 6.283185307179586) * 0.5;
        target_->set_requested_bounds({14.0 + travel * eased, 39.0, 168.0, 34.0});
        Panel::arrange(final_bounds);
    }

    void on_paint(Painter& painter, Rect damage) override {
        Panel::on_paint(painter, damage);
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        painter.draw_text_utf8({14.0, 23.0}, "WINDOW-QUEUE MOTION · 4 Hz",
                               {FontRole::control, 10.0, 700, false}, muted);
        painter.draw_line({14.0, 84.0}, {std::max(14.0, bounds.width - 14.0), 84.0},
                          rule, 1.0);
    }

private:
    std::shared_ptr<Button> target_;
    double phase_{};
};

struct ShowcaseContext final {
    std::vector<Control::Ptr> pages;
    std::vector<std::shared_ptr<Button>> navigation;
    std::vector<SubscriptionToken> subscriptions;
    std::shared_ptr<Label> status;
    std::shared_ptr<EasingBoard> easing;
    std::vector<std::shared_ptr<ProgressBar>> animated_progress;
    ComponentContainer components;
    std::shared_ptr<Timer> ui_timer;
    std::shared_ptr<ToolTip> tooltips;
    std::shared_ptr<Label> timer_status;
    std::shared_ptr<ProgressBar> timer_progress;
    std::shared_ptr<TrackBar> timer_interval;
    std::shared_ptr<TimerMotionBoard> timer_motion;
    std::shared_ptr<Button> timer_start;
    std::shared_ptr<Button> timer_stop;
    std::shared_ptr<Button> tooltip_show_disabled;
    std::shared_ptr<Button> tooltip_disabled_target;
    std::uint64_t timer_ticks{};
    std::size_t selected_page{};

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
        static constexpr std::array<std::string_view, 12> names{
            "Control spectrum", "Ranges and progress", "Containers and focus",
            "Animation laboratory", "States and diagnostics", "Text and input",
            "Collections and popups", "Values and spinners",
            "Images and drawing", "Tabs and pages", "Checked collections",
            "Timing and tooltips"};
        status->set_text(std::string(names[index]) +
                         "  ·  public GUI.Forms behavior  ·  CPU retained renderer");
    }
};

class ShowcaseRoot final : public LayoutPanel {
public:
    ShowcaseRoot(StableId stable_id, std::shared_ptr<ShowcaseContext> context)
        : LayoutPanel(std::move(stable_id), {1280.0, 820.0}),
          context_(std::move(context)) {
        set_background(canvas);
    }

    [[nodiscard]] std::shared_ptr<ShowcaseContext> context() const noexcept {
        return context_;
    }

private:
    std::shared_ptr<ShowcaseContext> context_;
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
    control->set_font({FontRole::control, 12.0, 700, false});
    return control;
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
                         "Lucida-style content role: editable and field text remains quiet and readable.",
                         12.0, 400, ink);
    typography->add_at(content, {24.0, 76.0, 760.0, 24.0});
    auto center = label("showcase.controls.type.center", "Centered label", 12.0, 600, accent);
    center->set_alignment(HorizontalAlignment::center);
    typography->add_at(center, {24.0, 108.0, 250.0, 24.0});
    static_cast<void>(context);
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
                                "ProgressBar · blocks, continuous, marquee, pulse",
                                {930.0, 280.0});
    page->add_at(progress_group, {24.0, 340.0, 930.0, 280.0});
    for (std::size_t index = 0; index < 4U; ++index) {
        auto caption = label("showcase.ranges.progress.caption." + std::to_string(index),
                             index == 0U ? "Blocks" : index == 1U ? "Continuous"
                                 : index == 2U ? "Marquee" : "Pulse", 11.0, 600, muted);
        progress_group->add_at(caption,
                               {24.0, 38.0 + static_cast<double>(index) * 52.0,
                                110.0, 24.0});
        auto progress = make_control<ProgressBar>(
            StableId("showcase.ranges.progress." + std::to_string(index)));
        progress->set_accessible_name(index == 0U ? "Blocks progress"
            : index == 1U ? "Continuous progress"
            : index == 2U ? "Marquee progress" : "Pulse progress");
        progress->set_visual_style(static_cast<ProgressBarVisualStyle>(index));
        progress->set_value(64.0);
        progress_group->add_at(progress,
                               {135.0, 40.0 + static_cast<double>(index) * 52.0,
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
    auto first_background = make_control<Panel>(
        StableId("showcase.containers.first.background"));
    first_background->set_background(Color::rgba(225, 235, 243));
    first_background->set_requested_bounds({0.0, 0.0, 310.0, 430.0});
    split->first_panel()->add_child(first_background);
    auto second_background = make_control<Panel>(
        StableId("showcase.containers.second.background"));
    second_background->set_background(paper);
    second_background->set_requested_bounds({0.0, 0.0, 620.0, 430.0});
    split->second_panel()->add_child(second_background);

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

    auto note = label("showcase.containers.note",
                      "The splitter is the public SplitContainer—not demo-painted geometry.",
                      11.0, 600, green);
    page->add_at(note, {26.0, 552.0, 700.0, 24.0});
}

void add_animation(const std::shared_ptr<Surface>& page,
                   const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.animation.heading", "ANIMATION LABORATORY", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 520.0, 34.0});
    page->add_at(label("showcase.animation.lead",
                       "Eight public easing curves, alternate direction, retained frame scheduling, hidden-page suspension.",
                       12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});
    auto easing = make_control<EasingBoard>(StableId("showcase.animation.easing"));
    page->add_at(easing, {24.0, 92.0, 930.0, 340.0});
    context->easing = easing;

    auto controls = group("showcase.animation.controls",
                          "Animation controls and motion policy", {930.0, 168.0});
    page->add_at(controls, {24.0, 454.0, 930.0, 168.0});
    auto pause = make_control<Button>(StableId("showcase.animation.pause"), "Pause motion");
    pause->set_visual_style(ButtonVisualStyle::accent);
    controls->add_at(pause, {24.0, 42.0, 160.0, 36.0});
    const std::weak_ptr<ShowcaseContext> weak_context = context;
    const std::weak_ptr<Button> weak_pause = pause;
    context->subscriptions.push_back(pause->clicked().subscribe(
        *easing, [weak_context, weak_pause](ButtonBase&) {
            const auto context = weak_context.lock();
            const auto pause = weak_pause.lock();
            if (!context || !pause) {
                return;
            }
            const bool paused = !context->easing->paused();
            context->easing->set_paused(paused);
            for (const auto& progress : context->animated_progress) {
                progress->set_animation_enabled(!paused);
            }
            pause->set_text(paused ? "Resume motion" : "Pause motion");
        }));
    auto copy = label("showcase.animation.policy",
                      "Animation is deadline-driven and quiescent when hidden or paused. No perpetual redraw loop.",
                      12.0, 400, ink);
    controls->add_at(copy, {210.0, 46.0, 650.0, 28.0});
    auto reduced = make_control<CheckBox>(StableId("showcase.animation.reduced"),
                                          "Reduced-motion substitution");
    reduced->set_indicator_style(ChoiceIndicatorStyle::toggle);
    controls->add_at(reduced, {24.0, 96.0, 310.0, 28.0});
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
    page->add_at(diagnostics, {494.0, 226.0, 460.0, 240.0});
    auto completeness = group("showcase.states.coverage", "Coverage contract",
                              {930.0, 134.0});
    page->add_at(completeness, {24.0, 488.0, 930.0, 134.0});
    auto coverage = label("showcase.states.coverage.copy",
                          "LIVE: public controls, visual variants, focus, capture, split panes, timelines, active surfaces, semantic snapshots, native accessibility.\n"
                          "OPEN: data-bound virtual collections and tree/grid families.",
                          11.0, 400, ink);
    coverage->set_text_wrapping(TextWrapping::word);
    coverage->set_vertical_alignment(VerticalAlignment::near);
    completeness->add_at(coverage, {24.0, 42.0, 860.0, 58.0});
    static_cast<void>(context);
}

void add_text_input(const std::shared_ptr<Surface>& page,
                    const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.text.heading", "TEXT AND INPUT", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 500.0, 34.0});
    page->add_at(label("showcase.text.lead",
                       "Unicode editing, directional selection, captured drag, caret deadlines, replacement ranges, and history.",
                       12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});

    auto editing = group("showcase.text.editing", "Editable TextBox states",
                         {930.0, 274.0});
    page->add_at(editing, {24.0, 92.0, 930.0, 274.0});
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

    auto state = label("showcase.text.state", "Selection: empty · History: clean",
                       11.0, 600, green);
    editing->add_at(state, {154.0, 202.0, 700.0, 26.0});
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
                          {930.0, 210.0});
    page->add_at(commands, {24.0, 390.0, 930.0, 210.0});
    const std::array<std::pair<std::string_view, double>, 4> command_specs{{
        {"Select all", 24.0}, {"Replace selection", 206.0},
        {"Undo", 430.0}, {"Redo", 612.0},
    }};
    std::vector<std::shared_ptr<Button>> command_buttons;
    for (std::size_t index = 0; index < command_specs.size(); ++index) {
        auto button = make_control<Button>(
            StableId("showcase.text.command." + std::to_string(index)),
            std::string(command_specs[index].first));
        button->set_visual_style(index == 1U ? ButtonVisualStyle::accent
                                             : ButtonVisualStyle::standard);
        commands->add_at(button, {command_specs[index].second, 42.0,
                                  index == 1U ? 196.0 : 154.0, 36.0});
        command_buttons.push_back(button);
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
    auto guarantee = label("showcase.text.guarantee",
                           "All editing state lives in public GUI.Forms TextBox + TextStore. The host only normalizes pointer, key, and text events.",
                           11.0, 600, shell_blue_dark);
    guarantee->set_text_wrapping(TextWrapping::word);
    guarantee->set_vertical_alignment(VerticalAlignment::near);
    commands->add_at(guarantee, {24.0, 104.0, 830.0, 60.0});
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
    context->timer_motion = make_control<TimerMotionBoard>(
        StableId("showcase.timing.motion"));
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

} // namespace

ShowcaseTree build_showcase_tree() {
    auto context = std::make_shared<ShowcaseContext>();
    auto root = make_control<ShowcaseRoot>(StableId("showcase.root"), context);

    auto header = make_control<Surface>(StableId("showcase.header"),
                                        Size{1280.0, 88.0}, SurfaceKind::header);
    root->add_at(header, {0.0, 0.0, 1280.0, 88.0});
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
                                         Size{238.0, 694.0}, SurfaceKind::sidebar);
    root->add_at(sidebar, {0.0, 88.0, 238.0, 694.0});
    auto nav_title = label("showcase.sidebar.title", "SHOWCASE INDEX", 12.0, 700,
                           shell_blue_dark);
    sidebar->add_at(nav_title, {20.0, 22.0, 190.0, 24.0});
    const std::array<std::string_view, 12> page_names{
        "01  Controls", "02  Ranges", "03  Containers",
        "04  Animation", "05  States", "06  Text & Input",
        "07  Collections", "08  Values", "09  Images & Drawing",
        "10  Tabs & Pages", "11  Checked Lists", "12  Timing & ToolTips"};
    for (std::size_t index = 0; index < page_names.size(); ++index) {
        auto navigation = make_control<Button>(
            StableId("showcase.navigation." + std::to_string(index)),
            std::string(page_names[index]));
        navigation->set_visual_style(ButtonVisualStyle::command);
        navigation->set_style(showcase_style());
        sidebar->add_at(navigation, {16.0, 58.0 + static_cast<double>(index) * 46.0,
                                     205.0, 35.0});
        context->navigation.push_back(navigation);
    }
    auto proof = label("showcase.sidebar.proof",
                       "No browser · No GPU · Retained core · Public controls",
                       11.0, 600, muted);
    proof->set_text_wrapping(TextWrapping::word);
    proof->set_vertical_alignment(VerticalAlignment::near);
    sidebar->add_at(proof, {20.0, 614.0, 196.0, 30.0});
    auto live = make_control<CheckBox>(StableId("showcase.sidebar.live"),
                                       "Live animation");
    live->set_checked(true);
    live->set_indicator_style(ChoiceIndicatorStyle::toggle);
    sidebar->add_at(live, {20.0, 662.0, 190.0, 28.0});

    auto content = make_control<Surface>(StableId("showcase.content"),
                                         Size{1042.0, 650.0}, SurfaceKind::page);
    root->add_at(content, {238.0, 88.0, 1042.0, 650.0});
    for (std::size_t index = 0; index < page_names.size(); ++index) {
        auto page = make_control<Surface>(
            StableId("showcase.page." + std::to_string(index)),
            Size{1000.0, 660.0}, SurfaceKind::page);
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

    auto status_surface = make_control<Surface>(StableId("showcase.status"),
                                                Size{1042.0, 44.0}, SurfaceKind::status);
    root->add_at(status_surface, {238.0, 738.0, 1042.0, 44.0});
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
            if (context->easing) {
                context->easing->set_paused(!enabled);
            }
            for (const auto& progress : context->animated_progress) {
                progress->set_animation_enabled(enabled);
            }
        }));
    context->select_page(0U);
    return {root};
}

void initialize_showcase_runtime(Window& window) {
    const auto root = std::dynamic_pointer_cast<ShowcaseRoot>(window.root());
    if (!root) throw std::logic_error("showcase runtime requires its retained root");
    const auto context = root->context();
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
        context->timer_motion->target(),
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
            context->timer_motion->set_phase(cycle);
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
